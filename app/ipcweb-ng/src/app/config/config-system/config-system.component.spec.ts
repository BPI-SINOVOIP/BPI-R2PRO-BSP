import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ConfigSystemComponent } from './config-system.component';

describe('ConfigSytemComponent', () => {
  let component: ConfigSystemComponent;
  let fixture: ComponentFixture<ConfigSystemComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ConfigSystemComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ConfigSystemComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
