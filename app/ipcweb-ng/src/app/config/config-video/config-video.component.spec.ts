import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ConfigVideoComponent } from './config-video.component';

describe('ConfigVideoComponent', () => {
  let component: ConfigVideoComponent;
  let fixture: ComponentFixture<ConfigVideoComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ConfigVideoComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ConfigVideoComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
