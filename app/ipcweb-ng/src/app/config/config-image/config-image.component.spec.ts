import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ConfigImageComponent } from './config-image.component';

describe('ConfigImageComponent', () => {
  let component: ConfigImageComponent;
  let fixture: ComponentFixture<ConfigImageComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ConfigImageComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ConfigImageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
